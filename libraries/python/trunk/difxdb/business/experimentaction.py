from difxdb.model import model
from sqlalchemy import desc


def experimentExists(session, code):
    '''
    Checks if an experiment with the given code exists. 
    Returns True if the experiment exists, False otherwise
    '''
    
    if (session.query(model.Experiment).filter_by(code=code).count() > 0):
        return(True)
    else:
        return(False)

def getLastExperimentNumber(session):
    '''
    Retrieves the highest experiment number assigned so far.
    '''
    exp = session.query(model.Experiment.number).order_by(desc(model.Experiment.number)).first()
    return(exp.number)
    
def getExperimentByCode(session, code):
    
    return(session.query(model.Experiment).filter_by(code=code).one())
    
def getActiveExperimentCodes(session):
    '''
    Determines all active experiments. Experiments are considered to be active if their state is not "released". 
    Returns a list containing the experiment codes
    '''
    result = []
    
    for instance in  session.query(model.Experiment).join(model.ExperimentStatus).filter(model.ExperimentStatus.statuscode<100).order_by(model.Experiment.code):
        result.append(instance.code)
        
    return(result)
 
def addExperiment(session, code):
    '''
    Adds an experiment to the database.
    '''
    
    if (experimentExists(session, code)):
        return

    experiment = model.Experiment()
    experiment.code = code
    experiment.number = int(getLastExperimentNumber(session)) + 1
    

    session.add(experiment)
    session.commit()